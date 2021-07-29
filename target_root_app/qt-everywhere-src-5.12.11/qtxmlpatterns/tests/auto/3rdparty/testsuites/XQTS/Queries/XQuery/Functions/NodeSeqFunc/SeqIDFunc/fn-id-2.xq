(: Name: fn-id-2 :)
(: Description: Evaluation of fn:id with context item not a node. :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   fn:id("argument1")
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()