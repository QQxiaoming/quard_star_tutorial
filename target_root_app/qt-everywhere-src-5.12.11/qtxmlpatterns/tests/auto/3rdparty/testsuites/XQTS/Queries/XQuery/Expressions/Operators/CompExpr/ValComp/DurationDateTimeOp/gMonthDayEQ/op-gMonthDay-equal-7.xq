(:*******************************************************:)
(:Test: op-gMonthDay-equal-7                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "and" expression (eq operator).          :)
(:*******************************************************:)
 
(xs:gMonthDay("--04-02Z") eq xs:gMonthDay("--02-03Z")) and (xs:gMonthDay("--01-07Z") eq xs:gMonthDay("--12-10Z"))