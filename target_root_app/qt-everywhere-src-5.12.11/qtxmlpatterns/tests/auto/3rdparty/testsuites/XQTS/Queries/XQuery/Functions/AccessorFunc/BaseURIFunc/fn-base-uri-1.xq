(: Name: fn-base-uri-1 :)
(: Description: Evaluation of base-uri function with no arguments and no context item :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:base-uri()
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()