(:Test: fn-implicit-timezone-2                           :)
(:Description: Normal call to "fn:implicit-timezone".     :)

(:insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:string(fn:implicit-timezone())