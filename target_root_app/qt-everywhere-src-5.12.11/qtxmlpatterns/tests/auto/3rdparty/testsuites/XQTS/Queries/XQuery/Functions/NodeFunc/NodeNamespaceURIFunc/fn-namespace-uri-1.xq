(: Name: fn-namespace-uri-1:)
(: Description: Evaluation of the fn:namespace-uri function with an undefined context node and no argument.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:namespace-uri()
};
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()