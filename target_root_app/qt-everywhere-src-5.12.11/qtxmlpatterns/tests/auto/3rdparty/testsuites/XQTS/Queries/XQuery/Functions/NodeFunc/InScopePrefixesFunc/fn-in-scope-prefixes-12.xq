(: Name: fn-in-scope-prefixes-12 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:lower-case.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:lower-case(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1])