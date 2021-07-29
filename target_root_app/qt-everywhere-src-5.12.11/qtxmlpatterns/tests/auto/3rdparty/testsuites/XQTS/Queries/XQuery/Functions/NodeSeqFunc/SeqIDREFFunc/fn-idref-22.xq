(: Name: fn-idref-22 :)
(: Description: Evaluation of fn:idref with context item not a node and second argument set to ".". :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:idref("argument1",.)
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()