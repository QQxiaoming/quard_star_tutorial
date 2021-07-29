(: Name: base-URI-3 :)
(: Description: Verify that a base-uri declaration containing "abc&gt;" is a valid base-uri declaration. :)

declare base-uri "http://www.example.com/abc&gt;";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"