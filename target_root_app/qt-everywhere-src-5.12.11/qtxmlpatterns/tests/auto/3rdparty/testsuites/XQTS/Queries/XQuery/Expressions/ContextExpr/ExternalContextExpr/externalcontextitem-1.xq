(: Name: externalcontextitem-1 :)
(: Description: Simple context item test (uses just "name" :)

declare namespace eg = "http://example.org";
declare function eg:noContextFunction()
 {
   name
};

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

eg:noContextFunction()