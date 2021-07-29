(: Name: fn-in-scope-prefixes-3 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element node.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:in-scope-prefixes(<anElement>some content</anElement>)