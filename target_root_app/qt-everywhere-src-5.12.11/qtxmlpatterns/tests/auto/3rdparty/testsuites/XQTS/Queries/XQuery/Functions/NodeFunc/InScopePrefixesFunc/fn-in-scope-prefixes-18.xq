(: Name: fn-in-scope-prefixes-18 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:string-join.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-join((fn:in-scope-prefixes(<anElement>Some content</anElement>),"xml"),"")