(: Name: fn-local-name-1:)
(: Description: Evaluation of the fn:local-name function with an undefined context node and no argument.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:local-name()
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()