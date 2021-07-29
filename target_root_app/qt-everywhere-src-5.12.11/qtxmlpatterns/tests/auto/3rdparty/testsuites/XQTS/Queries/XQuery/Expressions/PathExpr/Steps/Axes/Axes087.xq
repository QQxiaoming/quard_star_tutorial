(: Name: Axes087 :)
(: Description: Parent of text nodes :)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

<root>
{$input-context/doc/part/*/text()/..}
</root>