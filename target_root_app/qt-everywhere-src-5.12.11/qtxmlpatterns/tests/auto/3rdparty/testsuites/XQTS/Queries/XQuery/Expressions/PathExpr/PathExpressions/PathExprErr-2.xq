(:*******************************************************:)
(:Test: PathExprErr-2                                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: July 13, 2006                                    :)
(:Purpose: Evaluate error condition XPTY0018             :)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(<a>1</a>,<b>2</b>)/(if(position() eq 1) then . else data(.)) 