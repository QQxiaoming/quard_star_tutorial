(:*******************************************************:)
(:Test: op-gMonth-equal-7                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 16, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function used    :)
(:together with "and" expression (eq operator).          :)
(:*******************************************************:)
 
(xs:gMonth("--04Z") eq xs:gMonth("--02Z")) and (xs:gMonth("--01Z") eq xs:gMonth("--12Z"))