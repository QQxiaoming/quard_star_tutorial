(: Name: fn-string-1:)
(: Description: Evaluation of the fn:string function with no argument and no context item defined.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:string()
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()