#include <clib/seg_tree.h>
#include <stdlib.h>
#include <clib/types.h>
#include <clib/tree.h>
#include <clib/hash_table.h>
#include <clib/memory.h>
#include <clib/log.h>

#define DEFAULT_SEG_TREE_NODE_COUNT 512         //默认节点个数
typedef struct tree_key {
    uint start;
    uint end;
} tree_key_t;

struct _seg_tree {
    tree_t *tree;
    hash_table_t *value_set;
    seg_traverse_func traverse_func; //遍历的时候用到
    void *user_data;            //遍历的时候用到
};

static int key_equal(const void* a, const void* b, void *user_data)
{
    tree_key_t* v1 = (tree_key_t*) a;
    tree_key_t* v2 = (tree_key_t*) b;

    //v1包含v2
    if (v1->start <= v2->start && v1->end >= v2->end) {
        return 0;
    }
    //v2包含v1
    else if (v2->start <= v1->start && v2->end >= v1->end) {
        return 0;
    }
    //v2 > v1
    else if (v2->start > v1->end) {
        return -1;
    }
    //v1 > v2
    else if (v1->start > v2->end) {
        return 1;
    }

    //默认小于
    return -1;
}

//static void destroy_notify(void *data)
//{
//    tree_value_t* value_list = (tree_value_t*) data;
//    tree_value_t* temp;
//
//    while (value_list != NULL)
//    {
//        temp = value_list->next;
//        //待实现，多个data指向一个地方的时候，什么时候释放内存？
//        //free(value_list->data);
//        free(value_list);
//        value_list = temp;
//    }
//}

//static gint value_address_equal(gconstpointer a, gconstpointer b,
//        void *user_data)
//{
//    if (a > b)
//    {
//        return 1;
//    }
//    else if (a == b)
//    {
//        return 0;
//    }
//    else
//    {
//        return -1;
//    }
//}

seg_tree_t* seg_tree_new(destroy_func data_destroy_func)
{
    struct _seg_tree *seg_tree = (struct _seg_tree *) mem_alloc(
            sizeof(struct _seg_tree));

    seg_tree->tree = tree_new_full(DEFAULT_SEG_TREE_NODE_COUNT, key_equal, NULL,
            default_destroy_func, list_destroy_func);
    seg_tree->value_set = hash_table_new(direct_hash, direct_compare_func, NULL,
            data_destroy_func, NULL);

    return seg_tree;
}

struct adjust_tree_param {
    list_t *key_list;
    list_t *value_list;
    uint pre_index;
    bool pre_index_find;
    bool end_index_find;
    bool find_start;
    bool find_end;
    tree_key_t *key;
    void* data;
};

static void add_node(struct adjust_tree_param* param, uint start, uint end,
        void *data)
{
    tree_key_t *key = (tree_key_t *) mem_alloc(sizeof(tree_key_t));
    key->start = start;
    key->end = end;
    list_append(param->key_list, key);
    list_append(param->value_list, data);
}

static bool calc_new_node(void *key, void *value, void* user_data)
{
    struct adjust_tree_param* param = (struct adjust_tree_param*) user_data;
    tree_key_t *new_key = param->key;
    tree_key_t *old_key = (tree_key_t *) key;
    list_t *old_value = (list_t *) value;

    //在tree里找到过开始点，且还没遍历到开始
    if (!param->pre_index_find && param->find_start) {
        //找到开始点
        if (new_key->start == old_key->start) {
            param->pre_index = old_key->end + 1;
            param->pre_index_find = true;
        }
    }
    //在tree里没找到过开始点，且还没遍历到开始
    else if (!param->pre_index_find && !param->find_start) {
        //找到开始点
        if (new_key->start < old_key->start) {
            //待插入的点在最前面
            if (new_key->end < old_key->start) {
                add_node(param, new_key->start, new_key->end, param->data);
                param->pre_index_find = true;
                param->end_index_find = true;
                return true;
            }
            // [1, 3]插入到[2,4]这种情况
            else if (new_key->end == old_key->end) {
                add_node(param, new_key->start, old_key->start - 1,
                        param->data);
                param->pre_index_find = true;
                param->end_index_find = true;
                return true;
            }
            param->pre_index = old_key->end + 1;
            param->pre_index_find = true;
            add_node(param, new_key->start, old_key->start - 1, param->data);
            list_append(old_value, param->data);
        }
    }
    //已经找到开始点，但没找到结束点
    else if (param->pre_index_find && old_key->end < new_key->end) {
        if (param->pre_index < old_key->start) {
            add_node(param, param->pre_index, old_key->start - 1, param->data);
        }

        list_append(old_value, param->data);
        param->pre_index = old_key->end + 1;

    }
    //找到结束点
    else if (param->pre_index_find && old_key->end >= new_key->end) {
        if (!param->find_end) {
            add_node(param, param->pre_index, new_key->end, param->data);
        } else {
            if (param->pre_index < old_key->start) {
                add_node(param, param->pre_index, old_key->start - 1,
                        param->data);
            }
        }
        param->end_index_find = true;
        return true;
    }
    return false;
}

static void adjust_tree(tree_t *tree, tree_key_t* key, void *data,
        bool find_start, bool find_end)
{
    struct list_node *key_list_node;
    struct list_node *value_list_node;
    list_t *temp_list;
    struct adjust_tree_param param;

    param.key_list = list_new(8, NULL);
    param.value_list = list_new(8, NULL);
    param.pre_index_find = false;
    param.end_index_find = false;
    param.data = data;
    param.key = key;
    param.pre_index = 0;
    param.find_start = find_start;
    param.find_end = find_end;

    tree_foreach(tree, calc_new_node, &param);
    //没找到开始点
    if (!param.pre_index_find) {
        add_node(&param, key->start, key->end, data);
    } else if (!param.end_index_find) {
        add_node(&param, param.pre_index, key->end, data);
    }

    key_list_node = list_head(param.key_list);
    value_list_node = list_head(param.value_list);
    while (key_list_node != NULL) {
        temp_list = list_new(8, NULL);
        list_append(temp_list, value_list_node->data);
        tree_insert(tree, key_list_node->data, temp_list);
        value_list_node = value_list_node->next;
        key_list_node = key_list_node->next;
    }

    list_free(param.key_list);
    list_free(param.value_list);
}

static void divide_tree_item(tree_t *tree, bool new_value_in_head, uint new_key,
        void *data, tree_key_t *old_key, list_t *old_data_list)
{
    tree_key_t *key1, *key2;
    list_t *value;

    //新的数据插入到头部，并且在尾部重合
    //例如5 插入到 [1,5]
    if (new_value_in_head && new_key == old_key->end) {
        list_append(old_data_list, data);
        return;
    }
    //例如1 插入到 [1,5]
    else if (!new_value_in_head && new_key == old_key->start) {
        list_append(old_data_list, data);
        return;
    }

    //在中间
    tree_steal(tree, old_key, NULL, NULL);
    key1 = (tree_key_t *) mem_alloc(sizeof(tree_key_t));
    key2 = (tree_key_t *) mem_alloc(sizeof(tree_key_t));

    key1->start = old_key->start;
    key2->end = old_key->end;
    //这里new_key肯定小于old_key->end，等于的情况上面已经排除掉了
    if (new_value_in_head) {
        key1->end = new_key;
        key2->start = new_key + 1;
    }
    //这里new_key肯定大于old_key->start，同上
    else {
        key1->end = new_key - 1;
        key2->start = new_key;
    }

    value = list_duplicate(old_data_list, NULL);
    list_append(old_data_list, data);

    if (new_value_in_head) {
        tree_insert(tree, key1, old_data_list);
        tree_insert(tree, key2, value);
    } else {
        tree_insert(tree, key2, old_data_list);
        tree_insert(tree, key1, value);
    }

    mem_free(old_key);

    return;
}

void seg_tree_insert(seg_tree_t *seg_tree, uint start, uint end, void* data)
{
    list_t *lookup_value1, *lookup_value2;
    tree_key_t lookup_key1, lookup_key2;
    tree_key_t *orig_key1, *orig_key2;
    tree_key_t *new_key;
    tree_key_t key;
    // tree_value_t *new_value;
    bool find_end, find_start;
    list_t *data_list;

    void *orig_key;
    void *orig_value;

    if (start > end) {
        LOG(LOG_LEVEL_WARNING, "end must larger than start");
        return;
    }
    key.start = start;
    key.end = end;

    if (data != NULL) {
        if (!hash_table_lookup_extended(seg_tree->value_set, data, &orig_key,
                &orig_value)) {
            hash_table_insert(seg_tree->value_set, data, NULL);
        }
    }

    if (tree_nnodes(seg_tree->tree) == 0) {
        data_list = list_new(8, NULL);
        new_key = (tree_key_t *) mem_alloc(sizeof(tree_key_t));
        new_key->start = start;
        new_key->end = end;
        list_append(data_list, data);
        tree_insert(seg_tree->tree, new_key, data_list);
        return;
    }

    lookup_key1.start = start;
    lookup_key1.end = start;
    find_start = tree_lookup_extended(seg_tree->tree,
            (const void*) &lookup_key1, (void**) &orig_key1,
            (void**) &lookup_value1);
    lookup_key2.start = end;
    lookup_key2.end = end;
    find_end = tree_lookup_extended(seg_tree->tree, (const void*) &lookup_key2,
            (void**) &orig_key2, (void**) &lookup_value2);

    if (find_start && find_end) {
        if (orig_key1 == orig_key2) {
            //完全重叠
            if (orig_key1->start == start && orig_key1->end == end) {
                list_append(lookup_value1, data);
                return;
            }
            tree_steal(seg_tree->tree, orig_key1, NULL, NULL);
            //老的区间和新的区间开始地址不重叠
            if (orig_key1->start != start) {
                new_key = (tree_key_t *) mem_alloc(sizeof(tree_key_t));
                new_key->start = orig_key1->start;
                new_key->end = start - 1;

                tree_insert(seg_tree->tree, new_key,
                        list_duplicate(lookup_value1, NULL));
            }
            //老的区间和新的区间结束地址不重叠
            if (orig_key1->end != end) {
                new_key = (tree_key_t *) mem_alloc(sizeof(tree_key_t));
                new_key->start = end + 1;
                new_key->end = orig_key1->end;

                tree_insert(seg_tree->tree, new_key,
                        list_duplicate(lookup_value1, NULL));
            }

            new_key = (tree_key_t *) mem_alloc(sizeof(tree_key_t));
            new_key->start = start;
            new_key->end = end;
            list_append(lookup_value1, data);
            tree_insert(seg_tree->tree, new_key, lookup_value1);

            mem_free(orig_key1);

            return;
        } else {
            divide_tree_item(seg_tree->tree, false, start, data, orig_key1,
                    lookup_value1);
            divide_tree_item(seg_tree->tree, true, end, data, orig_key2,
                    lookup_value2);
        }
    } else {
        if (!find_start) {
            //tree_insert_node(tree, key->start, copy_from_value(value));
        } else if (find_start) {
            divide_tree_item(seg_tree->tree, false, start, data, orig_key1,
                    lookup_value1);
        }

        if (!find_end) {
            //tree_insert_node(tree, key->end, copy_from_value(value));
        } else if (find_end) {
            divide_tree_item(seg_tree->tree, true, end, data, orig_key2,
                    lookup_value2);
        }
    }

    adjust_tree(seg_tree->tree, &key, data, find_start, find_end);

}

list_t* seg_tree_lookup(seg_tree_t *seg_tree, const uint key)
{
    tree_key_t tree_key;

    tree_key.start = key;
    tree_key.end = key;

    return tree_lookup(seg_tree->tree, &tree_key);
}

void seg_tree_free(seg_tree_t *seg_tree)
{
    hash_table_free(seg_tree->value_set);
    tree_free(seg_tree->tree);

    mem_free(seg_tree);
}

static bool local_traverse_func(void *key, void* value, void* user_data)
{
    seg_tree_t *seg_tree = (seg_tree_t *) user_data;
    tree_key_t *tree_key = (tree_key_t *) key;
    list_t *tree_value = (list_t *) value;

    return seg_tree->traverse_func(tree_key->start, tree_key->end, tree_value,
            seg_tree->user_data);
}

void seg_tree_foreach(seg_tree_t *seg_tree, seg_traverse_func func,
        void* user_data)
{
    seg_tree->user_data = user_data;
    seg_tree->traverse_func = func;
    tree_foreach(seg_tree->tree, local_traverse_func, seg_tree);
}
