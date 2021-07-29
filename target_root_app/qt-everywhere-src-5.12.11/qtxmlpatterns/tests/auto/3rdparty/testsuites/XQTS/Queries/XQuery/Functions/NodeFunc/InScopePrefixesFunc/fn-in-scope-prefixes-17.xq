(: Name: fn-in-scope-prefixes-17 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:concat.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:concat(fn:in-scope-prefixes(<anElement>Some content</anElement>),"m")