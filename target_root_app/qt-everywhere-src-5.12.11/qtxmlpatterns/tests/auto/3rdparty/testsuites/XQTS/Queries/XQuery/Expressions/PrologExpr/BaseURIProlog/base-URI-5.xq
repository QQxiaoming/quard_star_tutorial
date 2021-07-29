(: Name: base-URI-5 :)
(: Description: Verify that a base-uri declaration containing "abc&quot;" is a valid base-uri declaration. :)

declare base-uri "http://www.example.com/abc&quot;";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"