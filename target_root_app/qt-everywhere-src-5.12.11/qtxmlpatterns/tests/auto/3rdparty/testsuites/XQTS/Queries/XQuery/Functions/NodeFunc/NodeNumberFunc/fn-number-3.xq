(: Name: fn-number-3:)
(: Description: Evaluation of the fn:number function with an undefined context node.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:number()
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()