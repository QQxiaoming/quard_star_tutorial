(: Name: fn-in-scope-prefixes-15 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:substring-before.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:substring-before(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1],"m")