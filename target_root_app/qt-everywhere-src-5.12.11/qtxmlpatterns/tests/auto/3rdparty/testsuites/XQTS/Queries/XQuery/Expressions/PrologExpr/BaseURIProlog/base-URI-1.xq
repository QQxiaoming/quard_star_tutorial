(: Name: base-URI-1 :)
(: Description: Test for declaration of base-uri twice. :)

declare base-uri "http://example.org";
declare base-uri "http://example.org";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"