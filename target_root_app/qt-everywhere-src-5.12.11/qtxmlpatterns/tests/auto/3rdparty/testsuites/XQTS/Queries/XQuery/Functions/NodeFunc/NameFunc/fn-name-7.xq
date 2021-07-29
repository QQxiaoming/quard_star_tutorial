(: Name: fn-name-7:)
(: Description: Evaluation of the fn:name function with an undefined context node.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:name()
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()