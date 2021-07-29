(:*******************************************************:)
(:Test: op-multiply-dayTimeDuration-14                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "multiply-dayTimeDuration" function used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P21DT08H12M") * 2.0) ne xs:dayTimeDuration("P08DT08H05M")