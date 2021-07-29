(:*******************************************************:)
(:Test: op-subtract-times-14                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 28, 2005                                    :)
(:Purpose: Evaluates The "subtract-times" function used  :)
(:together with the numeric-equal operator "ne".         :)
(:*******************************************************:)
 
(xs:time("13:00:00Z") - xs:time("17:00:00Z")) ne xs:dayTimeDuration("P10DT01H01M")