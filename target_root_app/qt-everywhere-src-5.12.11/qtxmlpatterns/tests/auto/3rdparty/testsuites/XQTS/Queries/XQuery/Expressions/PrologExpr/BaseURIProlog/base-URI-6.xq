(: Name: base-URI-6 :)
(: Description: Verify that a base-uri declaration containing "abc&apos;" is a valid base-uri declaration. :)

declare base-uri "http://www.example.com/abc&apos;";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"