(:*******************************************************:)
(:Test: op-gMonth-equal-8                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 16, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function used    :)
(:together with "and" expression (ne operator).          :)
(:*******************************************************:)
 
(xs:gMonth("--12Z") ne xs:gMonth("--03Z")) and (xs:gMonth("--05Z") ne xs:gMonth("--08Z"))