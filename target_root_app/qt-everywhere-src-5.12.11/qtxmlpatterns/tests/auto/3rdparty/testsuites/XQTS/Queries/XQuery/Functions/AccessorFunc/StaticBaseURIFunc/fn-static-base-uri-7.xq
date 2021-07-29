(: Name: fn-static-base-uri-7 :)
(: Description: Evaluation of fn:static-base-uri function using "ldap" scheme. :)
(: Uses fn:string .:)

declare base-uri "urn:oasis:names:specification:docbook:dtd:xml:4.1.2";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:static-base-uri())