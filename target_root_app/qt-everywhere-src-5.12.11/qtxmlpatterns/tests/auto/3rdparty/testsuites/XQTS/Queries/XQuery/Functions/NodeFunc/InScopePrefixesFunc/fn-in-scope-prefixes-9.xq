(: Name: fn-in-scope-prefixes-9 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element node without xmlns attribute and a prolog namesapce declaration.:)

declare namespace p1 = "http://www.example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:in-scope-prefixes(<anElement>Some content</anElement>)