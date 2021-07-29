(: Name: fn-root-1:)
(: Description: Evaluation of the fn:root function with no arguments and no context node set.:)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:root()
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()