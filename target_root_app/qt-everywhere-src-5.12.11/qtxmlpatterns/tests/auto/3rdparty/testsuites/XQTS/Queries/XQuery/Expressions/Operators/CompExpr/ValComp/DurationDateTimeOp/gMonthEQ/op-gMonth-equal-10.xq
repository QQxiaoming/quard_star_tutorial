(:*******************************************************:)
(:Test: op-gMonth-equal-10                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "gMonth-equal" function used    :)
(:together with "or" expression (ne operator).           :)
(:*******************************************************:)
 
(xs:gMonth("--06Z") ne xs:gMonth("--06Z")) or (xs:gMonth("--08Z") ne xs:gMonth("--09Z"))