/** 
 * @file eis.c
 *
 * @brief EIS equations.
 *
 * @details Complex impedance for:
 *  - resistance
 *  - capacitance
 *  - inductance
 *  - semi-infinite warburg
 *  - finite length warburg
 *  - finite space warburg 
 *
 * Copyright (C) 2020-2021  Milan Skocic.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>. 
 *
 * Author: Milan Skocic <milan.skocic@gmail.com>
 *
 */

#include "electrox.h"

/**
 * @brief Compute resistance impedance
 * @details Z = R
 * @param r Resistance in Ohms.
 * @param w Angular frequency in rad.s^-1.
 * @return Z Complex impedance in Ohms.
 */
double complex resistance(double r, double w)
{
    return (double complex)r;
}

void gsl_resistance(gsl_vector *p, gsl_vector *w, gsl_vector_complex *Z)
{

    size_t i;

    double complex z;
    gsl_complex _z;
    double wi;
    double r = gsl_vector_get(p, 0);

    for (i = 0; i < w->size; i++)
    {
        wi = gsl_vector_get(w, i);
        z = resistance(r, wi);
        _z = gsl_complex_rect(creal(z), cimag(z));
        gsl_vector_complex_set(Z, i, _z);
    }
}

/**
 * @brief Compute capacitance impedance
 * @details \f$ Z = \frac{1}{j C \omega} \f$
 * @param c Capacitance in F.
 * @param w Angular frequency in rad.s^-1.
 * @return Z Complex impedance in Ohms.
 */
double complex capacitance(double c, double w)
{
    return 1.0 / (I * c * w);
}

/**
 * @brief Compute inductance impedance
 * @details \f$ Z = j L \omega \f$
 * @param l Capacitance in H.
 * @param w Angular frequency in rad.s^-1.
 * @return Z Complex impedance in Ohms.
 */
double complex inductance(double l, double w)
{
    return I * l * w;
}

void gsl_inductance(gsl_vector *p, gsl_vector *w, gsl_vector_complex *Z)
{

    size_t i;

    double complex z;
    gsl_complex _z;
    double wi;
    double r = gsl_vector_get(p, 0);

    for (i = 0; i < w->size; i++)
    {
        wi = gsl_vector_get(w, i);
        z = inductance(r, wi);
        _z = gsl_complex_rect(creal(z), cimag(z));
        gsl_vector_complex_set(Z, i, _z);
    }
}

/**
 * @brief Compute semi-infinite warburg impedance
 * @details \f$ Z = \frac{\sigma}{\sqrt{\omega}} \cdot (1-j) \f$
 * @param sigma Pseudo-Resistance in Ohms.s^(1/2).
 * @param w Angular frequency in rad.s^-1.
 * @return Z Complex impedance in Ohms.
 */
double complex warburg(double sigma, double w)
{
    return sigma / sqrt(w) * (1 - I);
}

/**
 * @brief Compute finite length warburg impedance
 * @details \f$ Z = \frac{r}{\sqrt{j \tau \omega}} \cdot \tanh \sqrt{j \tau
 * \omega} \f$
 * @param r Resistance in Ohms.
 * @param tau Characteristic time in s.
 * @param w Angular frequency in rad.s^-1.
 * @return Z Complex impedance in Ohms.
 */
double complex finite_length_warburg(double r, double tau, double w)
{
    return r * ctanh(csqrt(I * tau * w)) / csqrt(I * tau * w);
}

/**
 * @brief Compute finite space warburg impedance
 * @details \f$ Z = \frac{r}{\sqrt{j \tau \omega}} \cdot \coth \sqrt{j \tau
 * \omega} \f$
 * @param r Resistance in Ohms.
 * @param tau Characteristic time in s.
 * @param w Angular frequency in rad.s^-1.
 * @return Z Complex impedance in Ohms.
 */
double complex finite_space_warburg(double r, double tau, double w)
{
    return r / (ctanh(csqrt(I * tau * w)) * csqrt(I * tau * w));
}

/**
 * @brief  EIS Element
 * @details Constructor of an EisElement.
 * @param name Pointer to the name
 * @param type Pointer to the type
 * @return EisElement Pointer to object EisElement
 */
EisElement *EisElement__new__(char *name, element_type type)
{
    /* Memory allocation */
    EisElement *self = (EisElement *)malloc(sizeof(EisElement));
    self->__del__ = &EisElement__del__;
    self->__init__ = &EisElement__init__;
    self->__init__(self, name, type);
    return self;
}

/**
 * @brief Initialize an EIS element
 * @details Initialize an EIS element
 * @param self Pointer to an object EisElement
 * @param name Pointer to the name
 * @param type Pointer to the type
 * @return None
 */
void EisElement__init__(EisElement *self, char *name, element_type type)
{
    self->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(self->name, name);
    self->type = type;

    switch (self->type)
    {
    case R:
        self->Z = &gsl_resistance;
        self->p = gsl_vector_alloc(1);
        break;
    case L:
        self->Z = &gsl_inductance;
        self->p = gsl_vector_alloc(1);
        break;
    default:
        self->Z = NULL;
        self->p = NULL;
        break;
    }
}

void EisElement__del__(EisElement *self)
{
    free(self->name);
    gsl_vector_free(self->p);
    free(self);
}

/**
 * @brief EisCircuit
 * @details Constructor of an EisCircuit
 * @param name Pointer to the name
 * @param repr Pointer to the representation
 */
EisCircuit *EisCircuit__new__(char *name, char *repr)
{
    EisCircuit *c = (EisCircuit *)malloc(sizeof(EisCircuit));
    EisCircuit__init__(c, name, repr);
    return c;
}

/**
 * @brief Initialize an EIS circuit
 * @details Initialize an EIS circuit
 * @param self Pointer to an object EisCircuit
 * @param name Pointer to the name
 * @param type Pointer to the string representation
 * @return None
 */
void EisCircuit__init__(EisCircuit *self, char *name, char *repr)
{
    int i;
    int N = 3;
    element_type type = R;
    printf("INIT CIRCUIT\n");
    char *names[3] = {"E1", "E2", "E3"};

    /* Process repr to get the number of elements 
     * Populate with EisElements
     */
    self->elements = (EisElement **)malloc(N * sizeof(EisElement *));
    for (i = 0; i < N; i++)
    {
        self->elements[i] = EisElement__new__(names[i], type);
    }
    self->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
    strcpy(self->name, name);
    self->repr = (char *)malloc(sizeof(char) * (strlen(repr) + 1));
    strcpy(self->repr, repr);
}