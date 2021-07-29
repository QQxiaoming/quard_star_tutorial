(:*******************************************************:)
(:Test: op-gMonthDay-equal-10                            :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 22, 2005                                    :)
(:Purpose: Evaluates The "gMonthDay-equal" function used :)
(:together with "or" expression (ne operator).           :)
(:*******************************************************:)
 
(xs:gMonthDay("--06-02Z") ne xs:gMonthDay("--06-09Z")) or (xs:gMonthDay("--08-06Z") ne xs:gMonthDay("--09-01Z"))