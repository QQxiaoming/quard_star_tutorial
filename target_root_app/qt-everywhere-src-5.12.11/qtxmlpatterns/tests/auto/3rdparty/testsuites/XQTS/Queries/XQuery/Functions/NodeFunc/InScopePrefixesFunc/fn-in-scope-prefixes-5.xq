(: Name: fn-in-scope-prefixes-5 :)
(: Description: Evaluation of fn:in-scope-prefixes function for a computed constructed element node with no namespace declaration.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:in-scope-prefixes(element anElement {"Some content"})