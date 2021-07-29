(: Name: fn-name-13:)
(: Description: Evaluation of the fn:name function used as argument to the fn-subtstring function. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)


for $h in ($input-context1/works/employee[2]) 
return fn:substring(fn:name($h),2)
