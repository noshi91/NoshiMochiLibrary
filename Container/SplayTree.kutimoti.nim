type
    SplayNode*[Key,T] = ref object
        ## Node for Splay Tree
        left* : SplayNode[Key,T]
        right* : SplayNode[Key,T]
        parent* : SplayNode[Key,T]
        key* : Key
        value* : T
        size* : int
    
    SplayTree*[Key,T] = object
        ## Self-Balancing Binary Search Tree
        root* : SplayNode[Key,T]
        comp* : proc(x : Key,y : Key) : bool

proc newSplayNode*[Key,T](key : Key , val : T) : SplayNode[Key,T] =
    ## Create new SplayNode. O(1)
    var x : SplayNode[Key,T] = new SplayNode[Key,T]
    x.left = nil
    x.right = nil
    x.parent = nil
    x.key = key
    x.value = val
    x.size = 1
    return x


proc newSplayTree*[Key,T](comp : proc(x : Key,y : Key) : bool) : SplayTree[Key,T] = 
    ## Create new Splay Tree. O(1)
    var splay : SplayTree[Key,T]
    splay.root = nil
    splay.comp = comp
    return splay

proc size_update[Key,T](x : var SplayNode[Key,T]) =
    if x != nil:
        x.size = 1 + x.left.node_size() + x.right.node_size()

proc set_left[Key,T](par : var SplayNode[Key,T] , x : var SplayNode[Key,T]) =
    if par != nil : 
        par.left = x
    if x != nil :
        x.parent = par
    par.size_update()

proc set_right[Key,T](par : var SplayNode[Key,T] , x : var SplayNode[Key,T]) =
    if par != nil : 
        par.right = x
    if x != nil :
        x.parent = par
    par.size_update()

proc node_size[Key,T](x : SplayNode[Key,T]) : int =
    if x == nil:
        return 0
    return x.size

proc zig[Key,T](x : var SplayNode[Key,T]) =
    var p = x.parent
    p.set_left(x.right)
    if p.parent != nil:
        if(p.parent.left == p):
            p.parent.set_left(x)
        else:
            p.parent.set_right(x)
    else:
        x.parent = nil
    x.set_right(p)

proc zag[Key,T](x : var SplayNode[Key,T]) =
    var p = x.parent
    p.set_right(x.left)
    if p.parent != nil:
        if(p.parent.left == p):
            p.parent.set_left(x)
        else:
            p.parent.set_right(x)
    else:
        x.parent = nil
    x.set_left(p)


proc splay*[Key,T](sp_tree : var SplayTree[Key,T],x : var SplayNode[Key,T]) =
    ## Splaying operation of node x. amortized O(logN)
    if x == nil:
        return
    while x.parent != nil:
        if x.parent.parent == nil:
            if x.parent.left == x:
                zig(x)
            else:
                zag(x)
        elif x.parent.parent.left == x.parent and x.parent.left == x:
            zig(x.parent)
            zig(x)
        elif x.parent.parent.left == x.parent and x.parent.right == x:
            zag(x)
            zig(x)
        elif x.parent.parent.right == x.parent and x.parent.right == x:
            zag(x.parent)
            zag(x)
        else:
            zig(x)
            zag(x)
    sp_tree.root = x


proc find*[Key,T](sp_tree : var SplayTree[Key,T] , key : Key) : bool =
    ## if the node with the key exists, splay it and return true. else splay the last node and return false. amortized O(logN)
    var z : SplayNode[Key,T] = sp_tree.root
    var p : SplayNode[Key,T] = nil
    while z != nil:
        p = z
        if sp_tree.comp(z.key,key):
            z = z.right
        elif sp_tree.comp(key,z.key):
            z = z.left
        else:
            sp_tree.splay(z)
            return true
    sp_tree.splay(p)
    return false


proc insert*[Key,T](sp_tree : var SplayTree[Key,T] , key : Key , val : T) =
    ## insert the node with key and val. if find the node with the key, do nothing. amortized O(logN)
    if(sp_tree.find(key)):
        return
    var z = newSplayNode(key,val)
    if sp_tree.root == nil:
        sp_tree.root = z
    elif sp_tree.comp(sp_tree.root.key,key):
        z.set_right(sp_tree.root.right)
        sp_tree.root.set_right(z)
    else:
        z.set_left(sp_tree.root.left)
        sp_tree.root.set_left(z)
    sp_tree.splay(z)


proc erase*[Key,T](sp_tree : var SplayTree[Key,T] , key : Key) : bool =
    ## erase the node with the key. amortized O(logN)
    if not sp_tree.find(key):
        return false
    var z = sp_tree.root
    if z.left == nil and z.right == nil:
        sp_tree.root = nil
    elif z.left == nil:
        sp_tree.root = z.right
        sp_tree.root.parent = nil
    elif z.right == nil:
        sp_tree.root = z.left
        sp_tree.root.parent = nil
    else:
        var lm = z.left
        while lm.right != nil:
            lm = lm.right
        z.left.parent = nil
        sp_tree.splay(lm)
        sp_tree.root = lm
        sp_tree.root.set_right(z.right)
    return true

proc size*[Key,T](sp_tree : SplayTree[Key,T]) : int =
    ## size of Splay Tree. O(1) 
    return sp_tree.root.node_size


proc nth_node*[Key,T](sp_tree : SplayTree[Key,T] , n : int) : ref int =
    ## return the nth node's key of key. if it doesn't exist, return nil. amortized O(logN)
    var now = n
    var z = sp_tree.root
    while z != nil:
        if z.left.node_size == now:
            var n = new int
            n[] = z.key
            return n
        if z.left.node_size < now:
            now -= z.left.node_size + 1
            z = z.right
        else:
            z = z.left
    return nil

