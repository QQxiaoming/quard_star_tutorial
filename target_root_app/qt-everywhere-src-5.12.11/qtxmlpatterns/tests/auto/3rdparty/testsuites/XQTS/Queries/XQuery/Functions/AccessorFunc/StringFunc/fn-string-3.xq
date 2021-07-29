(: Name: fn-string-3:)
(: Description: Evaluation of the fn:string function with argument set to "." and no context node set.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:string(.)
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()