(:*******************************************************:)
(:Test: op-add-dayTimeDurations-8                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-dayTimeDurations" function that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:dayTimeDuration("P10DT01H01M") + xs:dayTimeDuration("-P11DT02H02M"))