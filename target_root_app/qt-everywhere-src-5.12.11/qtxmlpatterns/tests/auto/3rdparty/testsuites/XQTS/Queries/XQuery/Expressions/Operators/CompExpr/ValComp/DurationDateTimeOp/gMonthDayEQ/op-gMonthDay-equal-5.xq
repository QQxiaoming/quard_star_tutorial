(:*******************************************************:)
(:Test: op-gMonthDay-equal-5                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function that :)
(:return false and used together with fn:not (eq operator):)
(:*******************************************************:)
 
fn:not(xs:gMonthDay("--11-10Z") eq xs:gMonthDay("--10-02Z"))