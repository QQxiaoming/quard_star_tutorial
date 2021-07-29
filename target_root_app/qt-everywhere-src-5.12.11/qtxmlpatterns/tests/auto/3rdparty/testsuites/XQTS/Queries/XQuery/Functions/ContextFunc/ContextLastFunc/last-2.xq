(: Name: last-2 :)
(: Description: Evaluation of "fn:last", where the context node is not defined. :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
  last()
};
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()