(: Name: internalcontextitem-1 :)
(: Description: Simple context item test (uses just "." :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   .
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()