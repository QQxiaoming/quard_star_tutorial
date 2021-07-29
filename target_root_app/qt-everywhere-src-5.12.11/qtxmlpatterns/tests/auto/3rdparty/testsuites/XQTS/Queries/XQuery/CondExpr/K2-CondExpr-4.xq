(:*******************************************************:)
(: Test: K2-CondExpr-4                                   :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the EBV from a node sequence. Sorting and deduplication isn't necessary in that case. :)
(:*******************************************************:)
(: insert-start :)
declare variable $input-context1 external;
if($input-context1//(employee[location = "Denver"]/ancestor::*))
then 1
else 3