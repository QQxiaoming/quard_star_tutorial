(: Name: fn-in-scope-prefixes-16 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:substring-after.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring-after(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1],"m")