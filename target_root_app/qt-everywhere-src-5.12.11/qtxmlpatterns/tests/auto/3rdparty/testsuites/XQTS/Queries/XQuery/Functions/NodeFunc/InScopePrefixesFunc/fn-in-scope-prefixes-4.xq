(: Name: fn-in-scope-prefixes-4 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element node with a namespace declaration.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:in-scope-prefixes(<anElement xmlns:p1 = "http://www.example.com">some content</anElement>)