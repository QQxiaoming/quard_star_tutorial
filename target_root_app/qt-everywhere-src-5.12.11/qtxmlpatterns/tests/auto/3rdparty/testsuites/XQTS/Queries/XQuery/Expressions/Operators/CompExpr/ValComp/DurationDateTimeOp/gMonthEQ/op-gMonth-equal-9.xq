(:*******************************************************:)
(:Test: op-gMonth-equal-9                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function used    :)
(:together with "or" expression (eq operator).           :)
(:*******************************************************:)
 
(xs:gMonth("--02Z") eq xs:gMonth("--02Z")) or (xs:gMonth("--06Z") eq xs:gMonth("--06Z"))