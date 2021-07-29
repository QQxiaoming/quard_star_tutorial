(: Name: fn-in-scope-prefixes-10 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:string-length.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-length(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1])