(: Name: fn-in-scope-prefixes-13 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element used as argument to fn:string-to-codepoints.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string-to-codepoints(fn:in-scope-prefixes(<anElement>Some content</anElement>)[1])