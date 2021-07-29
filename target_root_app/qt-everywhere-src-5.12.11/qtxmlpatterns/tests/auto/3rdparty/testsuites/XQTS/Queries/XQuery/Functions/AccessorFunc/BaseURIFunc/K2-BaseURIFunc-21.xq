(:*******************************************************:)
(: Test: K2-BaseURIFunc-21                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that the return value of document-uri() is of correct type. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

for $i in (1,  base-uri($input-context1), 3)
return typeswitch($i)
case xs:anyURI return "xs:anyURI"
case xs:integer return "xs:integer"
default return "FAILURE"