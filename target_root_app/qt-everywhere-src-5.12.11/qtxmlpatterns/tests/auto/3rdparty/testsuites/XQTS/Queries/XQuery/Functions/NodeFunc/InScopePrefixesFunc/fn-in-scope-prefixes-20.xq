(: Name: fn-in-scope-prefixes-20 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:contains.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:contains(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1],"l")