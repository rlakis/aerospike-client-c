# Operators

The Operators object provides functions to easily define operations to be performed on a record via the [`operate()`](client.md#operate) function.

Example:

```js
var op = aerospike.Operators

var ops = [
  op.append('a', 'hello'),
  op.append('a', 'world'),
<<<<<<< HEAD
  op.increment('b',10),
=======
  op.incr('b',10),
>>>>>>> d39fa88f028e87f1b5585798cd9f3a01d71ee656
  op.read('a'),
  op.read('b'),
]

client.operate(key, ops, callback)
```

<<<<<<< HEAD

<a name="Functions"></a>
## Functions

<!--
################################################################################
append()
################################################################################
-->
<a name="append"></a>

=======
## Functions

>>>>>>> d39fa88f028e87f1b5585798cd9f3a01d71ee656
### append(bin, value)

Append the value to the bin. The bin must contain either String or a Byte Array, and the value must be of the same type.

<<<<<<< HEAD
Parameters:

- `bin`         – The name of the bin to append a value to.
- `value`       – The value to append to the bin. 

```js
op.append('a', 'hello')
op.append('a', 'world')
```

<!--
################################################################################
increment()
################################################################################
-->
<a name="increment"></a>

### increment(bin, value)

Add the value to the bin. The bin must contain an Integer.

Parameters:

- `bin`         – The name of the bin to increment the value.
- `value`       – The value to increment the bin by.

```js
op.increment('b', 10)
```

<!--
################################################################################
prepend()
################################################################################
-->
<a name="prepend"></a>

=======
```js
operators.append('a', 'hello')
operators.append('a', 'world')
```

### incr(bin, value)

Add the value to the bin. The bin must contain an Integer.

```js
operators.incr('b', 10)
```

>>>>>>> d39fa88f028e87f1b5585798cd9f3a01d71ee656
### prepend(bin, value)

Prepend the value to the bin. The bin must contain either String or a Byte Array, and the value must be of the same type.

<<<<<<< HEAD
Parameters:

- `bin`         – The name of the bin to prepend a value to.
- `value`       – The value to prepend to the bin. 

```js
op.prepend('c', 'world')
op.prepend('c', 'hello')
```

<!--
################################################################################
read()
################################################################################
-->
<a name="read"></a>

### read(bin)

Read the value of the bin, after all other operations have completed.

Parameters:

- `bin` – The name of the bin to read the value of.

```js
op.read('d')
```

<!--
################################################################################
append()
################################################################################
-->
<a name="append"></a>

### touch(ttl=null)

Update the TTL for a record.

Parameters:

- `ttl` – (optional) The new relative TTL to set for the record, when it is touched.

```js
op.touch()
```

<!--
################################################################################
write()
################################################################################
-->
<a name="write"></a>

=======
```js
operators.prepend('c', 'world')
operators.prepend('c', 'hello')
```

### read(bin, value)

Read the value of the bin, after all other operations have completed.

```js
operators.read('d')
```

### touch([ttl])

Update the TTL for a record.

```js
operators.touch()
```

>>>>>>> d39fa88f028e87f1b5585798cd9f3a01d71ee656
### write(bin, value)

Update the value of the bin.

<<<<<<< HEAD
Parameters:

- `bin`         – The name of the bin to set the value for.
- `value`       – The value to set the bin to. 

```js
op.write('e','hello world')
=======

```js
operators.write('e','hello world')
>>>>>>> d39fa88f028e87f1b5585798cd9f3a01d71ee656
```
