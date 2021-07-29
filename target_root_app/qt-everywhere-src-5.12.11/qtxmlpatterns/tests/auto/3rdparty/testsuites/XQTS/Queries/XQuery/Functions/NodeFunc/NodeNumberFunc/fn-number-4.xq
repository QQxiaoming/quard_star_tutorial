(: Name: fn-number-4:)
(: Description: Evaluation of the fn:number function with an undefined context node and argument set to ".".:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:number(.)
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()