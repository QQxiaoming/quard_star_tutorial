(: Name: preceding-sibling-attr :)
(: Description: Apply the preceding-sibling axis to an attribute node. :)

<foo a='1' b='2' c='3'>
    <bar>4</bar>
    <bar>5</bar>
    <bar>6</bar>
</foo>/@c/preceding-sibling::node()
