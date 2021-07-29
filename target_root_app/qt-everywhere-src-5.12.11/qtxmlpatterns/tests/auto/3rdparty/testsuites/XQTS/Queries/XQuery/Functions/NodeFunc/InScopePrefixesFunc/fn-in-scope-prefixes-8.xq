(: Name: fn-in-scope-prefixes-8 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a directly constructed element node with xmlns attribute and a prolog namesapce declaration.:)

declare namespace p1 = "http://www.example.com";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:in-scope-prefixes(<anElement xmlns:p1="http://www.somenamespace.com">Some content</anElement>)