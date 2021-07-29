(:*******************************************************:)
(:Test: op-gMonthDay-equal-8                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "and" expression (ne operator).          :)
(:*******************************************************:)
 
(xs:gMonthDay("--12-10Z") ne xs:gMonthDay("--03-11Z")) and (xs:gMonthDay("--05-12Z") ne xs:gMonthDay("--08-08Z"))