(: Name: base-URI-2 :)
(: Description: Verify that a base uri declaration containing "abc&lt;" is a valid base-uri declaration. :)

declare base-uri "http://www.example.com/abc&lt;";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"