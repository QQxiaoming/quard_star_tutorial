(: Name: base-URI-4 :)
(: Description: Verify that a base-uri declaration containing "abc&amp;" is a valid base-uri declaration. :)

declare base-uri "http://www.example.com/abc&amp;";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"