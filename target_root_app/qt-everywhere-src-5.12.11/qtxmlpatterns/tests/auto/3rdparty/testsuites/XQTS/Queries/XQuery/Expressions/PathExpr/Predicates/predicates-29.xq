(: Name: predicates-29:)
(: Description: A negative test for numeric range used as filter expression. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $foo := <element1><element2>some content</element2></element1>
return $foo[(2 to 5)]