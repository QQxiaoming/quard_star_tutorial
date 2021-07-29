(:*******************************************************:)
(: Test: K2-SeqIDFunc-10                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Select elements with xml:id, and trigger node sorting. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

id(("short", "positiveInteger"), $input-context1)