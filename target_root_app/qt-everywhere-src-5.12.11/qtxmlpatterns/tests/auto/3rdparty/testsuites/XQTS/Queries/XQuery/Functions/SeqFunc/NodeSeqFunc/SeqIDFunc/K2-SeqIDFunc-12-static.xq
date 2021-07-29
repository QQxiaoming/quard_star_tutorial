(:*******************************************************:)
(: Test: K2-SeqIDFunc-12                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Select elements with xml:id, trigger node sorting, and use an invalid NCName in the lookup. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

id((".", "short", "123"), exactly-one($input-context1//xs:element/@name[. = "positiveInteger"]))
