(:*******************************************************:)
(:Test: op-subtract-times-15                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 28, 2005                                    :)
(:Purpose: Evaluates The "subtract-times" function used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:time("13:00:00Z") - xs:time("17:00:00Z")) le xs:dayTimeDuration("P10DT02H10M")