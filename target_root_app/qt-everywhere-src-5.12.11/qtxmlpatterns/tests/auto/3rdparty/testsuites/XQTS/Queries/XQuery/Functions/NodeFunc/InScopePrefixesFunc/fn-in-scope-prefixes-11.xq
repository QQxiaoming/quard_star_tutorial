(: Name: fn-in-scope-prefixes-11 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:upper-case.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:upper-case(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1])