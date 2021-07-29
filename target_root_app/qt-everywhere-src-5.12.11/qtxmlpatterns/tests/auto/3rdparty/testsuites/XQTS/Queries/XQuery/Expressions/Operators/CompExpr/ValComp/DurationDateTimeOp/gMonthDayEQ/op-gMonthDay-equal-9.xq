(:*******************************************************:)
(:Test: op-gMonthDay-equal-9                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "or" expression (eq operator).           :)
(:*******************************************************:)
 
(xs:gMonthDay("--02-01Z") eq xs:gMonthDay("--02-10Z")) or (xs:gMonthDay("--06-02Z") eq xs:gMonthDay("--06-03Z"))