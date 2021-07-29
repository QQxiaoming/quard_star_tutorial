(:*******************************************************:)
(:Test: op-subtract-times-13                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 28, 2005                                    :)
(:Purpose: Evaluates The "subtract-times" function used  :)
(:together with the numeric-equal- operator "eq".        :)
(:*******************************************************:)
 
(xs:time("13:00:00Z") - xs:time("17:00:00Z")) eq xs:dayTimeDuration("P20DT01H02M")